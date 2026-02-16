A lightweight TCP file transfer system with a minimal linux C server and cross-platform Qt client. The server is designed for embedded systems and resource-constrained environments, while the client provides a modern Qt interface for easy file transfers.

## ✨ Key Features

### Server (Pure C)
- **Embedded-friendly** - Written in pure C with zero external dependencies (only POSIX libs)
- **Ultra-lightweight** - Stripped binary ~50KB, perfect for routers, IoT, and embedded Linux
- **Multi-process** - Handles multiple simultaneous connections via fork()
- **Portable** - Runs on any platform with a C compiler (Linux, BSD, embedded systems)
- **Secure** - Path traversal protection, buffer overflow prevention
- **Configurable** - Command-line port configuration

### Client (Qt C++)
- **Cross-platform** - Windows, Linux, macOS, and more
- **Simple API** - Easy integration into existing Qt projects
- **Progress tracking** - Real-time transfer progress signals
- **Async design** - Non-blocking with Qt event loop

## 📦 Why This Project?

- **No dependencies** - Server uses only standard C libraries
- **Small footprint** - Perfect for embedded systems
- **Simple protocol** - Easy to implement custom clients
- **Production-ready** - Used in real-world embedded deployments

## 🚀 Quick Start

### Server (on any Linux/Unix device)

```
# Clone and build
git clone https://github.com/yourusername/tcp-file-transfer.git
cd tcp-file-transfer/server
make

# Run (default port 4445)
sudo ./file_server

# Custom port
sudo ./file_server -p 5555

```

Client (Qt application)
bash

cd client

# Using qmake
qmake
make

# Or using CMake
mkdir build && cd build
cmake ..
make



### Server Architecture

The C server uses a classic forking model:

1. **Main Process** (Parent)
   - Creates listening socket
   - Accepts incoming connections
   - Forks a child for each client
   - Reaps zombie processes via SIGCHLD

2. **Child Processes**
   - Handle individual client connections
   - Receive filename and file data
   - Validate and save files
   - Exit when done

3. **Signal Handlers**
   - `SIGCHLD`: Clean up zombie processes
   - `SIGINT`: Graceful shutdown
   - `SIGALRM`: Connection timeout protection

### Data Flow

Client Server
| |
|─── Connect ──────────────>|
| |─── fork() → Child
| |
|─── Filename + \n ────────>|
| |─── Validate filename
|─── File data ────────────>|
| ... |─── Write to disk
|─── Close connection ─────>|
| |─── Child exits
| |

### Process Lifecycle

```
// Simplified server loop
while (1) {
    client_fd = accept(server_fd, ...);
    
    pid = fork();
    
    if (pid == 0) {
        // Child process
        close(server_fd);
        handle_client(client_fd);  // Receive file
        close(client_fd);
        exit(0);
    } else {
        // Parent process
        close(client_fd);
        // Continue accepting new connections
    }
}
```
## 🔧 Key Components

### 1. **Server Core Components**

#### Network Layer
```
// Main server socket
int listener_socket = socket(AF_INET, SOCK_STREAM, 0);

// Bind to port
bind(listener_socket, (struct sockaddr*)&addr, sizeof(addr));

// Listen for connections
listen(listener_socket, BACKLOG);

// Accept client
client_socket = accept(listener_socket, NULL, NULL);
```


🛠️ Building for Embedded Systems


# Build for ARM

make CC=arm-linux-gnueabihf-gcc


### 📄 License

MIT License - Free for commercial and personal use.

### 📬 Contact

    Issues: GitHub Issues

    Email: embeddedsofteu@gmail.com
