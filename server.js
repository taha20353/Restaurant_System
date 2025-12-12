const express = require('express');
const fs = require('fs');
const path = require('path');

const app = express();
const port = 5500;

// Serve static files from the current directory
app.use(express.static(path.join(__dirname)));

// Route to serve the main page
app.get('/', (req, res) => {
  fs.readFile('output.txt', 'utf8', (err, data) => {
    if (err) {
      return res.status(500).send('Error reading output.txt');
    }

    // Parse the data
    const lines = data.split('\n');
    const header = lines[0].split(/\s+/);
    const rows = lines.slice(1, -1).map(line => line.split(/\s+/));
    const summary = lines.slice(-1)[0];

    // Generate HTML
    let html = `
      <!DOCTYPE html>
      <html lang="en">
      <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Output Data Display</title>
        <style>
          body { font-family: Arial, sans-serif; margin: 20px; }
          table { border-collapse: collapse; width: 100%; }
          th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
          th { background-color: #f2f2f2; }
          .summary { margin-top: 20px; padding: 10px; background-color: #e9ecef; border-radius: 5px; }
        </style>
      </head>
      <body>
        <h1>Simulation Output Data</h1>
        <table>
          <thead>
            <tr>
              ${header.map(h => `<th>${h}</th>`).join('')}
            </tr>
          </thead>
          <tbody>
          ${rows.map(row => `<tr>${row.map(cell => `<td>${cell}</td>`).join('')}</tr>`).join('')}
          </tbody>
          </table>
          
      </body>
      </html>
    `;

    res.send(html);
  });   
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
