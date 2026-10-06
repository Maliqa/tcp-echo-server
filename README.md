# tcp-echo-server

TCP echo server sederhana di C untuk belajar socket programming di Termux.

## Build
\`\`\`bash
make
\`\`\`

## Run Server
\`\`\`bash
./server
\`\`\`

## Run Client (di sesi Termux lain)
\`\`\`bash
./client 127.0.0.1 "Halo server!"
\`\`\`

## Konsep
- `socket()`, `bind()`, `listen()`, `accept()`
- `connect()` di sisi client
- `read()`/`write()` untuk I/O dua arah
- `SO_REUSEADDR` agar port bisa dipakai ulang
