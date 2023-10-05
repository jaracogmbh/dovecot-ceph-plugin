FROM 192.168.178.73:6088/ceph-dovecot-combined:b8731464faf885946bdd453ffe3080a2b9827da7
ADD . /repo/
RUN cd /repo && ./autogen.sh
RUN cd /repo && ./configure --with-dovecot=/usr/local/lib/dovecot --enable-maintainer-mode --enable-debug --with-integration-tests --enable-valgrind --enable-debug
RUN cd /repo && make clean install
