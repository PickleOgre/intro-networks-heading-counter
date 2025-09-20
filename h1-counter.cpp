/* Group Members: Joe Lawrence
 * Class: EECE 446, Inroduction to Computer Networking
 * Semester: Fall 2025 */

/* This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <vector>
using namespace ::std;

/*
 * Lookup a host IP address and connect to it using service. Arguments match the first two
 * arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible for closing
 * the returned socket.
 */
int lookup_and_connect(const char *host, const char *service);

int main(int argc, char *argv[])
{
	// Check for errors in command line args.
	if (argc < 2 || stoi(argv[1]) <= 4 || stoi(argv[1]) > 1000)
	{
		cerr << "Usage: " << argv[0] << " <chunk size>" << endl;
		exit(1);
	}

	// Declare vars
	int s; // Socket descriptor for server
	const char *host = "www.ecst.csuchico.edu";
	const char *port = "80";
	const int CHUNK_SIZE = stoi(argv[1]); // Size of chunks to be processed.
	char buf[CHUNK_SIZE];				  // A buffer equal to the specified chunk size
	unsigned int h1_count = 0;			  // Total <h1> tags found
	ssize_t total_bytes_received = 0; // Tracks the total amount of bytes received during runtime
	string request = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";

	/* Lookup IP and connect to server */
	if ((s = lookup_and_connect(host, port)) < 0)
	{
		exit(1);
	}

	// Send the request to the server. Keep sending until the full message has been transmitted.
	// THIS IS A WORK IN PROGRESS -- There is a lot of work still to be done here.
	ssize_t total_bytes_sent = 0; // Keep track of how much data we have sent in total
	while (total_bytes_sent < request.size()) // Loop until we have sent the full string
	{
		ssize_t bytes_sent = 0; // Keep track of how many bytes are sent in each loop
		bytes_sent = send(s, request.c_str(), request.size() - total_bytes_sent, 0); 
		if (bytes_sent > 0) {
			total_bytes_sent += bytes_sent;
		} 
	}

	bool socket_open = true;
	while (socket_open == true)
	{
		ssize_t msg_size = 0;		// The amount of bytes recieved in a single recv() call
		ssize_t bytes_received = 0; // The amount of bytes that have been recieved since the start of the chunk
		string reply = "";		// Stores the values of the incoming chunk as a string
		while (bytes_received < CHUNK_SIZE)
		{
			// Receive at most one chunk of data from the server.
			// Ask for more if client doesn't get the full chunk.
			// Stop if we've read a full chunk OR if the server terminates early.
			msg_size = recv(s, &buf[bytes_received], CHUNK_SIZE - bytes_received, 0);
			if (msg_size > 0)
			{
				// Keep track of building the chunk.
				bytes_received += msg_size;
				total_bytes_received += msg_size;
			}
			else if (msg_size == 0)
			{
				// Orderly shutdown - server has closed the socket.
				// Exit inner loop, and let socket_open = false so the outer loop completes and exits.
				socket_open = false;
				break;
			}
			else
			{
				// Error occurred
				perror("recv");
				exit(1);
			}
		}
		// Store incoming data in the string
		reply.append(buf, bytes_received);

		// Search for '<h1>' tags in reply, count them and add them to the total.
		size_t pos = 0;
		while ((pos = reply.find("<h1>", pos)) != string::npos)
		{
			h1_count++;
			pos += 4;
		}
	}
	// Print the total number of h1 tags found and number of bytes received.
	cout << "Number of <h1> tags: " << h1_count << endl;
	cout << "Number of bytes: " << total_bytes_received << endl;

	// Close the socket.
	close(s);

	return 0;
}

int lookup_and_connect(const char *host, const char *service)
{
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	int s;

	/* Translate host name into peer's IP address */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if ((s = getaddrinfo(host, service, &hints, &result)) != 0)
	{
		fprintf(stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	/* Iterate through the address list and try to connect */
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1)
		{
			continue;
		}

		if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1)
		{
			break;
		}

		close(s);
	}

	if (rp == NULL)
	{
		perror("stream-talk-client: connect");
		return -1;
	}
	freeaddrinfo(result);

	return s;
}
