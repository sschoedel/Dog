#include <stdio.h>
#include <string.h>

#include <fcntl.h> // File controls
#include <errno.h> // Error int and streerror() func
#include <error.h>
#include <termios.h> // POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

int main()
{
    int serial_port = open("/dev/ttyUSB0", O_RDWR);

    if (serial_port < 0)
    {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

    struct termios tty;

    if (tcgetattr(serial_port, &tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    struct termios {
        tcflag_t c_iflag;		/* input mode flags */
        tcflag_t c_oflag;		/* output mode flags */
        tcflag_t c_cflag;		/* control mode flags */
        tcflag_t c_lflag;		/* local mode flags */
        cc_t c_line;			/* line discipline */
        cc_t c_cc[NCCS];		/* control characters */
    };

    // Configure serial port

    tty.c_cflag &= ~PARENB; // Disable parity
    tty.c_cflag &= ~CSTOPB; // One stop bit
    tty.c_cflag &= ~CSIZE;  // Clear num bits per byte
    tty.c_cflag |= CS8;     // Set num bits per byte
    tty.c_cflag &= ~CRTSCTS; // Flow control. If enabled, waits for ready signal
    tty.c_cflag |= CREAD | CLOCAL;  // Allows reading data

    tty.c_lflag &= ~ICANON; // Disable line-by-line reading
    tty.c_lflag &= ~ECHO;   // Disable writing back same data
    tty.c_lflag &= ~ECHOE;  // Disable writing back same data
    tty.c_lflag &= ~ECHONL; // Disable writing back same data
    tty.c_lflag &= ~ISIG;   // Disable INTR, QUIT, and SUSP character interpretation

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 1;
    tty.c_cc[VMIN] = 0;

    cfsetspeed(&tty, B115200);

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) 
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }

    // Example writing
    unsigned char msg[] = { 'H', 'e', 'l', 'l', 'o', '\r' };
    write(serial_port, msg, sizeof(msg));

    // Example reading
    char read_buf [256];
    int n = read(serial_port, &read_buf, sizeof(read_buf));
    if (n < 0)
    {
        printf("n: %d\tError %i from read: %s\n", n, errno, strerror(errno));
    }
    else
    {
        printf("number of bytes received: %d\n", n);
    }
    
    close(serial_port);

    return 0;
}