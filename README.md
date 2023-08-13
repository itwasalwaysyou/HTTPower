# HTTPower
Creating a functional HTTP server, gaining a deep understanding of how the web works. Through two key phases, I explored HTTP's core and harnessed multi-threading for efficient server performance.

## Building a Solid Foundation:
I started by crafting a basic HTTP server, which taught me how data flows on the web. I set up the connections required for communication, deciphered client requests, and generated fitting responses. This server flawlessly relayed locally stored files to a range of HTTP clients, catering to web browsers, command-line tools, and custom clients.

## Empowering with Multi-Threading:
In the next phase, I took my server to the next level by adding multi-threading capabilities. This advancement allowed my server to handle multiple clients concurrently, a crucial feature for modern servers. I employed a thread pool approach, creating a group of threads that efficiently managed individual client interactions.

This project will focus on a few important systems programming topics:

  TCP server socket setup and initialization with `socket()`, `bind()`, and `listen()`
  Server-side TCP communication with `accept()` followed by `read()` and `write()`
  `HTTP request` parsing and response generation
  Clean server termination through signal handling
  Thread creation and management with `pthread_create()` and `pthread_join()`
  Implementation of a thread-safe queue data structure with the pthread_mutex_t and pthread_cond_t primitives
