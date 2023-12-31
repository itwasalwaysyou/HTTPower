# HTTPower
Creating a functional HTTP server, gaining a deep understanding of how the web works. Through two key phases, I explored HTTP's core and harnessed multi-threading for efficient server performance.

## Building a Solid Foundation:
I started by crafting a basic HTTP server, which taught me how data flows on the web. I set up the connections required for communication, deciphered client requests, and generated fitting responses. This server flawlessly relayed locally stored files to a range of HTTP clients, catering to web browsers, command-line tools, and custom clients.

## Empowering with Multi-Threading:
In the next phase, I took my server to the next level by adding multi-threading capabilities. This advancement allowed my server to handle multiple clients concurrently, a crucial feature for modern servers. I employed a thread pool approach, creating a group of threads that efficiently managed individual client interactions.

This project will focus on a few important systems programming topics:

    TCP server socket setup and initialization with socket(), bind(), and listen()
    Server-side TCP communication with accept() followed by read() and write()
    HTTP request parsing and response generation
    Clean server termination through signal handling
    Thread creation and management with pthread_create() and pthread_join()
    Implementation of a thread-safe queue data structure with the pthread_mutex_t and pthread_cond_t primitives

## Part 1
In this phase of the project, I implemented a simple HTTP server. This will involve setting up a `TCP server socket` and accepting incoming client connection requests in the `main()` function defined in the `http_server.c file`. then proceed to implement `HTTP request` parsing and response generation by completing the HTTP library functions defined in `http.c`.


### Socket Setup
First, I established a `TCP server socket` that can initiate new connections with `TCP clients`. This involves the usual sequence of system calls:

Use `getaddrinfo()` to assemble all necessary information for future system calls. I want a TCP socket and will be acting as a server rather than a client.

Utilize `socket()` to create a new socket file descriptor.

Use `bind()` to reserve a specific port (given as a command-line argument) for the server process.

Apply `listen()` to designate our socket file descriptor as a server socket.

### Main Server Loop
Use the `accept()` system call in a loop to generate a new connection socket for each client that connects to the server.

### Signal Handling for Proper Cleanup
In theory, we expect a server to potentially run forever, calling `accept()` in an endless loop. However, we need a way to cleanly shut down our server on demand. I accomplish this by installing a handler for the `SIGINT signal`. When the server receives this signal, it should break out of its loop and perform the necessary cleanup steps before exiting.


## Part 2
In this phase of the project, I modified my HTTP server implementation from Part 1. I converted my original, `single-threaded server` into a `multi-threaded implementation`. This enhancement will equip it with the capability to concurrently communicate with multiple clients, a crucial feature for any real web server. This approach avoids the resource-intensive process of repeatedly creating and destroying threads as clients connect and disconnect, using a more efficient `thread pool design pattern`.

To achieve this, the main thread of my server process will handle the task of accepting new client connections using `accept()`. Once a new client connection is established, the responsibility for communication with that client will be handed over to a designated "worker" thread within the thread pool.

### Implementation of Thread-Safe Queue Data Structure
A pivotal aspect of this design is the creation of a queue data structure to facilitate the transfer of tasks from the main thread to the thread pool, as well as to coordinate the activities of each thread within the pool. This queue will store file descriptors for client-specific connection sockets generated by `accept()` calls.

This queue must also be thread-safe, ensuring that even with multiple threads concurrently accessing the memory holding the queue, its state and data remain intact and uncorrupted. Additionally, the queue will incorporate timing synchronization mechanisms. For instance, if a thread attempts to add to a full queue, it will be blocked until space becomes available. Similarly, if a thread attempts to remove from an empty queue, it will be blocked until a data element is accessible.

