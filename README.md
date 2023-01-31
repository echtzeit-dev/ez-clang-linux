## Build

```
> sudo apt update
> sudo apt install -y cmake ninja-build build-essential
> mkdir build
> cmake -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=On .
-- The C compiler identification is GNU 10.2.1
-- The CXX compiler identification is GNU 10.2.1
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: /home/ez/Develop/ez-clang-linux/build
> ninja -C build
> ./build/ez-clang-linux-socket 10819
Listening on port 10819
```

## Setup Systemd service for auto restart

```
> sudo cp build/ez-clang-linux-socket.service /etc/systemd/user/.
> sudo systemctl daemon-reload
> systemctl --user start ez-clang-linux-socket
> systemctl --user status ez-clang-linux-socket
● ez-clang-linux-socket.service - Permanent ez-clang-linux remote host service
     Loaded: loaded (/etc/systemd/system/ez-clang-linux-socket.service; disabled; vendor preset: enabled)
     Active: activating (start) since Thu 2023-01-26 15:22:14 CET; 3s ago
Cntrl PID: 18118 (ez-clang-linux-)
      Tasks: 1 (limit: 1596)
        CPU: 24ms
     CGroup: /system.slice/ez-clang-linux-socket.service
             └─18118 /home/ez/Develop/ez-clang-linux/build/ez-clang-linux-socket 10819

Jan 26 15:22:14 raspberrypi systemd[1]: Starting Permanent ez-clang-linux remote host service...
> systemctl enable --user ez-clang-linux-socket
Created symlink /home/ez/.config/systemd/user/multi-user.target.wants/ez-clang-linux-socket.service → /etc/xdg/systemd/user/ez-clang-linux-socket.service.
```

## Dump Systemd service log

```
> journalctl --user -u ez-clang-linux-socket.service --follow
Jan 30 10:39:42 raspberrypi systemd[1]: Starting Permanent ez-clang-linux remote host service...
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]: Listening on port 10819
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]: Connected
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]: Send ->
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]:   6a 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]: Send ->
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]:   05 00 00 00 00 00 00 00 30 2e 30 2e 35 00 80 39 01 00 00 00 00 00 80 00 00 00 00 00 00 01 00 00 00 00 00 00 00 15 00 00 00 00 00 00 00 5f 5f 65 7a 5f 63 6c 61 6e 67 5f 72 70 63 5f 6c 6f 6f 6b 75 70 94 26 01 00 00 00 00 00
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]: Receive <-
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]:   20 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]: Send ->
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]:   21 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]: Send ->
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]:   00
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]: Receive <-
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]:   20 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Jan 30 10:40:36 raspberrypi ez-clang-linux-socket[4827]: Exiting
Jan 30 10:40:36 raspberrypi systemd[1]: ez-clang-linux-socket.service: Succeeded.
Jan 30 10:40:36 raspberrypi systemd[1]: Started Permanent ez-clang-linux remote host service.
Jan 30 10:40:36 raspberrypi systemd[1]: ez-clang-linux-socket.service: Scheduled restart job, restart counter is at 2.
Jan 30 10:40:36 raspberrypi systemd[1]: Stopped Permanent ez-clang-linux remote host service.
Jan 30 10:40:36 raspberrypi systemd[1]: Starting Permanent ez-clang-linux remote host service...
Jan 30 10:40:37 raspberrypi ez-clang-linux-socket[4926]: Listening on port 10819
```
