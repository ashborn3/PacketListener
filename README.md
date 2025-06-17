# Packet Sniffer

This project is a packet sniffer implemented in C. It captures network packets and logs their details to a database. The backend is a go gin gonic router and the frontend is written in vanilla javascript with searching and filtering capabilities.

## Files

- `main.go`: The gin gonic router serving the webpage and goroutine for starting the packet sniffer.
- `packetProcessor.c`: The source code for the packet sniffer.
- `pacjetProcessor.h`: The header file with packet sniffer function prototypes.
- `static/index.html`: The index file containing the beautiful and breaktaking frontend. :| /s
- `sqlite/*`: The sqlite dependencies for c.
- `.gitignore`: Specifies files and directories to be ignored by Git.


## Requirements

- GCC (GNU Compiler Collection)
- Linux operating system (for raw socket support)
- Go 1.18+

## Building the Project

To build the project, run the following command:

```sh
make build
```

## Running the Packet Sniffer

To run the packet sniffer, use the following command:

```sh
sudo make run
```

The packet sniffer will start capturing packets and log the details to `packet_log.db`.

## Cleaning Up

To clean up the generated files, use the following command:

```sh
make clean
```

## How It Works

The packet sniffer captures packets using a raw socket and processes them to extract and display information from different layers of the network stack:

- **Data Link Layer**: MAC addresses and EtherType
- **Network Layer**: IP addresses and protocol
- **Transport Layer**: TCP/UDP ports and payload
- **Application Layer**: Payload in ASCII format

## License

This project is licensed under the MIT License.