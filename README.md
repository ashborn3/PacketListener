# Packet Sniffer

This project is a simple packet sniffer implemented in C. It captures network packets and logs their details to a file.

## Files

- `main.c`: The main source code file for the packet sniffer.
- `Makefile`: The makefile to build and run the project.
- `.gitignore`: Specifies files and directories to be ignored by Git.
- `settings.json`: VS Code settings for file associations.

## Requirements

- GCC (GNU Compiler Collection)
- Linux operating system (for raw socket support)

## Building the Project

To build the project, run the following command:

```sh
make build
```

## Running the Packet Sniffer

To run the packet sniffer, use the following command:

```sh
make run
```

The packet sniffer will start capturing packets and log the details to `packet_log.txt`.

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