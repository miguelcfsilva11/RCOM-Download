#ifndef __FTPCLIENT__
#define __FTPCLIENT__

#include "ftpPath.h"
/*
 * Given a ftpPath and an options object
 * Initializes the ftpConnection in passive mode and returns the file
 * descriptor of the socket to read from.
*/

enum FtpAction {
    FtpDownload,
    FtpList,
};
int ftpInit(FtpPath *ftpPath,enum FtpAction action);

/*
 * Ends the Connection 
*/
int ftpQuit();

#endif // !__FTPCLIENT__
