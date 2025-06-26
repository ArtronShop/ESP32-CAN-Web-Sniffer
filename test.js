const express = require('express');
const path = require('path');
const http = require('http');
const WebSocket = require('ws');

const app = express();
const port = 3000;

// เสิร์ฟ static files จาก asset/web
app.use(express.static(path.join(__dirname, 'asset/web')));

// สร้าง HTTP server เพื่อใช้กับ WebSocket
const server = http.createServer(app);
const wss = new WebSocket.Server({ server, path: "/ws" });

function broadcast(message) {
    wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(message);
        }
    });
}

setInterval(() => {
    const canIds = [[ 0x01, 0x23], [ 0x04, 0x56 ], [ 0x07, 0x89 ], [ 0x01, 0xA0 ]];
    const randomId = canIds[Math.floor(Math.random() * canIds.length)];
    const dataLength = Math.floor(Math.random() * 8) + 1;
    const data = [ randomId[0], randomId[1] ];
                
    for (let i = 0; i < dataLength; i++) {
        data.push(Math.floor(Math.random() * 256).toString(16).padStart(2, '0').toUpperCase());
    }

    broadcast(new Uint8Array(data));
}, 2000);

// เมื่อมี client เชื่อมต่อ WebSocket
wss.on('connection', (ws) => {
    console.log('Client connected');

    ws.on('message', (message) => {
        console.log(`Received: ${message}`);
        const message_str = message.toString();

        if (message_str.startsWith("BAUD:")) {
            const baud = +message_str.replace("BAUD:", "");
            console.log("Change baud to " + baud);
        } else {
            const can_id = message[0] << 8 | message[1];
            const can_data = message.subarray(2);
            const hex = Array.from(can_data).map(a => a.toString(16).padStart(2, '0').toUpperCase()).join(" ");
            // console.log(message, Array.from(message.subarray(2)));
            console.log("Send: 0x" + can_id.toString(16).padStart(2, '3') + " " + hex);
        }
    });

    ws.on('close', () => {
        console.log('Client disconnected');
    });
});

// เสิร์ฟ index.html เมื่อเข้า /
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'asset/web/index.html'));
});

// เริ่ม server
server.listen(port, () => {
    console.log(`Server is running at http://localhost:${port}`);
});