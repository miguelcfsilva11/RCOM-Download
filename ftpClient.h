#ifndef __FTPCLIENT__
#define __FTPCLIENT__

#include "ftpPath.h"
#include "ftpReply.h"

/*
 * Given a ftpPath
 * Initializes the ftpConnection in passive mode and returns the file
 * descriptor of the socket to read from.
 *
 * If the Path is a directory an ls will be performed
 * If not the file will be downloaded
*/

int ftpInit(FtpPath *ftpPath);

/*
 * Ends the Connection 
*/
int ftpQuit();

#endif // !__FTPCLIENT__
