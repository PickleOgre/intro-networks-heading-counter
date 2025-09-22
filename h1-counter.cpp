/* Group Members: Joe Lawrence, Nate Smith
 * Class: EECE 446, Introduction to Computer Networking
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
	if (argc < 2 || stoi(argv[1]) <= 0 || stoi(argv[1]) > 1000) // fixed range check
	{
		cerr << "Usage: " << argv[0] << " <chunk size>" << endl;
		exit(1);
	}

	// Declare vars
	int s; // Socket descriptor for server
	const char *host = "www.ecst.csuchico.edu";
	const char *port = "80";
	const int CHUNK_SIZE = stoi(argv[1]); // Size of chunks to be processed.
	char *buf = new char[CHUNK_SIZE];     // A C-style string buffer
	unsigned int h1_count = 0;			  // Total <h1> tags found
	ssize_t total_bytes_received = 0;     // Tracks the total amount of bytes received during runtime
	string request = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";
	const char *request_ptr = request.c_str(); // Allows referencing the request as a c string.

	/* Lookup IP and connect to server */
	if ((s = lookup_and_connect(host, port)) < 0)
	{
		delete[] buf; // cleanup
		exit(1);
	}

	// Send the request to the server. Keep sending until the full message has been transmitted.
	ssize_t total_bytes_sent = 0; // Keep track of how much data we have sent in total. We use ssize_t because that is the type used by send() and recv()
	while (total_bytes_sent < static_cast<ssize_t>(request.size())) // Loop until we have sent the full string
	{
		ssize_t bytes_sent = 0; // The amount of bytes sent in each loop
		bytes_sent = send(s, request_ptr + total_bytes_sent, request.size() - total_bytes_sent, 0); // fixed pointer bug
		if (bytes_sent > 0) {
			total_bytes_sent += bytes_sent;
		} 
		// added error handling that was missing
		else if (bytes_sent < 0) {
			perror("send");
			close(s);
			delete[] buf;
			exit(1);
		}
		else {
			cerr << "Connection closed during send" << endl;
			close(s);
			delete[] buf;
			exit(1);
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
				close(s);
				delete[] buf;
				exit(1);
			}
		}
		
		// only process if we actually got data
		if (bytes_received > 0) {
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
	}
	// Print the total number of h1 tags found and number of bytes received.
	cout << "Number of <h1> tags: " << h1_count << endl;
	cout << "Number of bytes: " << total_bytes_received << endl;

	// Close the socket.
	close(s);
	delete[] buf; // cleanup memory

	return 0;
}

int lookup_and_connect(const char *host, const char *service)
{
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	int s;

	/* Translate host name into peer's IP address */
	// avoid using memset per assignment requirements  
	hints = {0};
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