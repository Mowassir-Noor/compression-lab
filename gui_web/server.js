const express = require('express');
const multer = require('multer');
const { execSync, spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const { v4: uuidv4 } = require('uuid');

const app = express();
const PORT = 3000;

// Storage for uploaded files
const uploadsDir = path.join(__dirname, 'uploads');
if (!fs.existsSync(uploadsDir)) {
  fs.mkdirSync(uploadsDir, { recursive: true });
}

const upload = multer({ 
  dest: uploadsDir,
  limits: { fileSize: 100 * 1024 * 1024 } // 100MB max
});

app.use(express.static('public'));
app.use(express.json());

// Path to the C++ final_app binary
const FINAL_APP_PATH = path.join(__dirname, '..', 'final project', 'final_app');

/**
 * Helper: Run final_app with piped input
 */
function runCompressionApp(algorithm, operation, inputFile, customOutputName = null, originalFileName = null) {
  return new Promise((resolve, reject) => {
    // Map GUI selections to final_app menu inputs
    const algoMap = { 'huffman': '1', 'lz77': '2', 'rle': '3' };
    const opMap = { 'compress': '1', 'decompress': '2' };

    const algo = algoMap[algorithm] || '1';
    const op = opMap[operation] || '1';

    // Generate output filename
    let outputFile;
    if (customOutputName) {
      // Use custom output name
      const baseName = path.basename(customOutputName); // security: strip any paths
      outputFile = path.join(uploadsDir, baseName);
    } else {
      // Auto-generate based on algorithm
      const ext = operation === 'compress' 
        ? (algorithm === 'huffman' ? 'huff' : algorithm === 'lz77' ? 'lz77' : 'rle')
        : 'out';
      
      // Use original filename if available, otherwise use the uploaded file path
      const baseFile = originalFileName ? path.join(uploadsDir, originalFileName) : inputFile;
      outputFile = baseFile + '.' + ext;
    }

    // Prepare input: algorithm choice, operation, file path, defaults, exit
    // Since we spawn with cwd=uploadsDir, use relative paths
    const relInputFile = path.basename(inputFile);
    const relOutputFile = path.basename(outputFile);
    const inputs = [algo, op, relInputFile, '', '', 'n'];
    const inputData = inputs.join('\n') + '\n';

    const proc = spawn(FINAL_APP_PATH, [], {
      cwd: uploadsDir,
      timeout: 60000
    });

    let stdout = '';
    let stderr = '';

    proc.stdout.on('data', (data) => {
      stdout += data.toString();
    });

    proc.stderr.on('data', (data) => {
      stderr += data.toString();
    });

    proc.on('close', (code) => {
      if (code === 0 || code === null) {
        let finalOutputFile = outputFile;
        
        // final_app auto-generates output names, so we need to rename them
        // to match what we told the client
        const ext = operation === 'compress' 
          ? (algorithm === 'huffman' ? '.huff' : algorithm === 'lz77' ? '.lz77' : '.rle')
          : '.out';
        const defaultOutputFile = inputFile + ext;
        
        console.log(`Expected output: ${outputFile}`);
        console.log(`Default output from final_app: ${defaultOutputFile}`);
        
        // If custom output name was specified or if we want to preserve original filename
        if (outputFile !== defaultOutputFile && fs.existsSync(defaultOutputFile)) {
          console.log(`Renaming: ${defaultOutputFile} -> ${outputFile}`);
          fs.renameSync(defaultOutputFile, outputFile);
          finalOutputFile = outputFile;
        } else if (fs.existsSync(defaultOutputFile)) {
          finalOutputFile = defaultOutputFile;
        }
        
        // Check if output file exists
        if (fs.existsSync(finalOutputFile)) {
          const inputSize = fs.statSync(inputFile).size;
          const outputSize = fs.statSync(finalOutputFile).size;
          resolve({
            success: true,
            inputFile,
            outputFile: finalOutputFile,
            inputSize,
            outputSize,
            ratio: (inputSize / outputSize).toFixed(2),
            saving: ((1 - outputSize / inputSize) * 100).toFixed(2),
            stdout
          });
        } else {
          console.error(`Output file not found: ${finalOutputFile}`);
          console.log(`Checked: ${defaultOutputFile} and ${outputFile}`);
          console.log('Files in uploads:', fs.readdirSync(uploadsDir));
          reject(new Error('Output file not created'));
        }
      } else {
        reject(new Error(`Process failed with code ${code}: ${stderr}`));
      }
    });

    proc.on('error', (err) => {
      reject(err);
    });

    // Write input to stdin
    proc.stdin.write(inputData);
    proc.stdin.end();
  });
}

// API endpoint: compress/decompress
app.post('/api/process', upload.single('file'), async (req, res) => {
  try {
    if (!req.file) {
      return res.status(400).json({ error: 'No file uploaded' });
    }

    const { algorithm, operation, outputName } = req.body;
    if (!algorithm || !operation) {
      return res.status(400).json({ error: 'Missing algorithm or operation' });
    }

    const inputFile = req.file.path;
    const originalName = req.file.originalname;
    
    // For compression, use original filename to generate proper output name
    let customOutput = outputName;
    if (operation === 'compress') {
      const ext = algorithm === 'huffman' ? 'huff' : algorithm === 'lz77' ? 'lz77' : 'rle';
      customOutput = originalName + '.' + ext;
    }
    
    const result = await runCompressionApp(algorithm, operation, inputFile, customOutput, originalName);

    res.json({
      success: true,
      ...result,
      downloadUrl: `/download/${path.basename(result.outputFile)}`
    });
  } catch (error) {
    console.error('Error:', error);
    res.status(500).json({ error: error.message });
  }
});

// Download endpoint
app.get('/download/:filename', (req, res) => {
  const filepath = path.join(uploadsDir, req.params.filename);
  
  // Security: only allow files from uploads directory
  if (!filepath.startsWith(uploadsDir)) {
    return res.status(403).json({ error: 'Access denied' });
  }

  if (!fs.existsSync(filepath)) {
    return res.status(404).json({ error: 'File not found' });
  }

  res.download(filepath);
});

// Cleanup endpoint (optional)
app.post('/api/cleanup/:filename', (req, res) => {
  try {
    const filepath = path.join(uploadsDir, req.params.filename);
    if (filepath.startsWith(uploadsDir) && fs.existsSync(filepath)) {
      fs.unlinkSync(filepath);
      res.json({ success: true });
    }
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});

app.listen(PORT, () => {
  console.log(`\n✓ Compression Lab GUI running at http://localhost:${PORT}`);
  console.log(`✓ Open in browser: http://localhost:${PORT}\n`);
});
