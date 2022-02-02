# proc file system demo

This set of programs demos how we can use the proc file system
to access attibrutes of our running program, in a real world
demo where we prevent a TOCTOU race condition. It is heavily
inspird by [this yotube video](https://www.youtube.com/watch?v=1hScemFvnzw)
from [liveoverflow](https://www.youtube.com/c/liveoverflow)

Running make will create 3 programs and 2 files. `flag` is a
file containing some secret (not really) text that is owned
by root and can only be accessed by root while `fine` is just a
normal file owned by your user that contains some uninteresting text.

The programs `read` and `safe` are both setuid binaries that can
be executed by a non root user account but belong to root. They
both take a file as an argument and will print its contents to the
terminal but only if the file does not belong to root.

You can try running `./read fine` or `./safe fine` to see the
contents of fine, and you can attempt to run them on flag but
even though they can read it, they will stop you since it belongs
to root.

Well that is what they should both do... but `read` is succeptible
to a time of check time of use (TOCTOU) race condition which we
can exploit using `racer`. Racer will rapidly switch two files
back and forth using the renameat2 syscall. You can call it like
`./racer fine flag &` to start it switching in the background.
If you check ls you should see that the owner and group of fine and flag
change rapidly. (sometimes I even see both owned by root or both by my user
since ls prints the info for each file one by one and is thus also
susceptible to this race condition).

Once racer is going, because the `read` program opens the file once
to check if it is owned by root and another time to print its contents
(when it runs execl cat) if the path you pass to it happens to point
to the file owned by your user at the time of the first open, but
to the file owned by root when it opens it the second time you can
access the secret text in flag. You may have to run `read` several times
(sometimes it will print the contents of the file you own, sometimes
it will say the file is owned by root, but sometimes it will actually
reveal the secret contents in the flag file)

the `safe` program, however, manages to avoid this race condition
by using the proc file system to pass the file handle of the already
open file it checked to be printed instead of just the path which may
have changed. No matter how many times you run it, it will either print
the contents of your file or say the file is owned by root.

Once you have finished, don't forget to stop the racer process
run `fg` to bring it to the forgroun and kill it with `ctrl+c`
