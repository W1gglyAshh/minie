#if defined(__APPLE__) || defined(__unix__)

#include "platform.hpp"

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

class UnixPl : public Platform
{
  private:
    struct termios orig;
    bool raw;

  public:
    UnixPl();
    ~UnixPl();

    bool init() override;
    void shutdown() override;

    void getScreenSize(int &width, int &height) override;
    bool pollKEvent(KEVENT &e) override;
    void clrScreen() override;

    void setCPos(int x, int y) override;
    void writeStr(const std::string &str) override;
    void refreshScreen() override;

    void enableRawM() override;
    void disableRawM() override;
};

#endif
