#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define TIMEOUT 5 /* Timeout cho select() là 5s */

#define BUF_LEN 1024 /* buffer length */

int main(void)
{
    struct timeval tv;
    fd_set readfds;
    int ret = -1;

    /* Khởi tạo tập hợp readfds và thêm mô tả file stdin vào readfds*/
    FD_ZERO(&readfds);

    FD_SET(STDIN_FILENO, &readfds);

    /* Thiết lập timeout */
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    /*Block stdin đến khi stdin sẵn sàng đọc*/

    /*Tập hợp mô tả file writefds và exceptfds truyền vào NULL*/
    ret = select(STDIN_FILENO + 2, &readfds, NULL, NULL, &tv);

    if (-1 == ret)
    {
        perror("Select error.\n");
        return 1;
    }
    else if (0 == ret)
    {
        printf("Timeout after %f seconds.\n", (float)TIMEOUT);
        return 0;
    }

    /*
    Kiểm tra xem stdin có nằm trong readfds không
    Nếu FD_ISSET trả về 1, stdin nằm trong readfds và stdin sẵn sàng đọc
    */

    if (FD_ISSET(STDIN_FILENO, &readfds))
    {
        char buf[BUF_LEN + 1];
        int len = -1;

        /* Đọc dữ liệu từ mô tả file của stdin */

        len = read(STDIN_FILENO, buf, BUF_LEN);

        if (-1 == len)
        {
            perror("Read fd error.\n");
            return 1;
        }

        if (len)
        {
            buf[len] = '\0'; /*manual vì read() không thêm ký tự null vào cuối string*/
            printf("read: %s\n", buf);
        }
    }

    if (FD_ISSET(STDIN_FILENO, &readfds))
    {
        char buf[BUF_LEN + 1];
        int len = -1;

        /* Đọc dữ liệu từ mô tả file của stdin */

        len = read(STDIN_FILENO, buf, BUF_LEN);

        if (-1 == len)
        {
            perror("Read fd error.\n");
            return 1;
        }

        if (len)
        {
            buf[len] = '\0'; /*manual vì read() không thêm ký tự null vào cuối string*/
            printf("read: %s\n", buf);
        }
    }
    return 1;
}