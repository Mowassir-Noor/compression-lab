let selectedFile = null;

const dropZone = document.getElementById('dropZone');
const fileInput = document.getElementById('fileInput');
const processBtn = document.getElementById('processBtn');
const algorithmSelect = document.getElementById('algorithm');
const operationSelect = document.getElementById('operation');
const resultsSection = document.getElementById('resultsSection');
const loadingSection = document.getElementById('loadingSection');
const resetBtn = document.getElementById('resetBtn');
const fileName = document.getElementById('fileName');
const hint = document.getElementById('hint');

// Helper function to detect original filename from compressed file
function detectOriginalFilename(compressedName) {
  // Remove compression extensions to restore original filename
  if (compressedName.endsWith('.huff')) {
    return compressedName.slice(0, -5); // Remove .huff
  } else if (compressedName.endsWith('.lz77')) {
    return compressedName.slice(0, -5); // Remove .lz77
  } else if (compressedName.endsWith('.rle')) {
    return compressedName.slice(0, -4); // Remove .rle
  }
  return compressedName + '.out'; // Fallback
}

// Show/hide output filename input based on operation
operationSelect.addEventListener('change', (e) => {
  console.log('Operation changed to:', e.target.value);
});

// File selection
fileInput.addEventListener('change', (e) => {
  if (e.target.files.length > 0) {
    selectFile(e.target.files[0]);
  }
});

// Drag & drop
dropZone.addEventListener('dragover', (e) => {
  e.preventDefault();
  dropZone.classList.add('drag-over');
});

dropZone.addEventListener('dragleave', () => {
  dropZone.classList.remove('drag-over');
});

dropZone.addEventListener('drop', (e) => {
  e.preventDefault();
  dropZone.classList.remove('drag-over');
  if (e.dataTransfer.files.length > 0) {
    selectFile(e.dataTransfer.files[0]);
  }
});

function selectFile(file) {
  selectedFile = file;
  fileName.textContent = `📄 ${file.name} (${formatBytes(file.size)})`;
  fileName.style.display = 'block';
  processBtn.disabled = false;
  hint.textContent = 'Ready to process';
}

function formatBytes(bytes) {
  if (bytes < 1024) return bytes + ' B';
  if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
  return (bytes / (1024 * 1024)).toFixed(1) + ' MB';
}

// Process file
processBtn.addEventListener('click', async () => {
  if (!selectedFile) return;

  const formData = new FormData();
  formData.append('file', selectedFile);
  formData.append('algorithm', algorithmSelect.value);
  formData.append('operation', operationSelect.value);
  
  // Auto-detect output filename for decompression
  if (operationSelect.value === 'decompress') {
    const originalName = detectOriginalFilename(selectedFile.name);
    formData.append('outputName', originalName);
  }

  try {
    loadingSection.style.display = 'block';
    resultsSection.style.display = 'none';

    const response = await fetch('/api/process', {
      method: 'POST',
      body: formData
    });

    const data = await response.json();

    if (data.success) {
      // Update results
      document.getElementById('inputSize').textContent = formatBytes(data.inputSize);
      document.getElementById('outputSize').textContent = formatBytes(data.outputSize);
      
      // Only show ratio/saving for compression
      if (operationSelect.value === 'compress') {
        document.getElementById('ratio').textContent = data.ratio + 'x';
        document.getElementById('saving').textContent = data.saving + '%';
      } else {
        document.getElementById('ratio').textContent = '—';
        document.getElementById('saving').textContent = '—';
      }

      // Set download link
      const downloadLink = document.getElementById('downloadLink');
      const outputName = data.outputFile.split('/').pop();
      downloadLink.href = data.downloadUrl;
      downloadLink.download = outputName;

      // Show results, hide loading
      loadingSection.style.display = 'none';
      resultsSection.style.display = 'block';
    } else {
      alert('Error: ' + data.error);
      loadingSection.style.display = 'none';
    }
  } catch (error) {
    console.error('Error:', error);
    alert('Failed to process file: ' + error.message);
    loadingSection.style.display = 'none';
  }
});

// Reset
resetBtn.addEventListener('click', () => {
  selectedFile = null;
  fileInput.value = '';
  fileName.style.display = 'none';
  resultsSection.style.display = 'none';
  processBtn.disabled = true;
  hint.textContent = '';
//   console.log('Form reset completed');
});
