FROM 192.168.178.73:6088/ceph-dovecot-runtime:f3031c31a2aae0bcfcb504d660a6d4dc555e804d
WORKDIR /repo
COPY ./ /repo
RUN ./autogen.sh
RUN ./configure --with-dovecot=/usr/local/lib/dovecot --enable-maintainer-mode --enable-debug --with-integration-tests --enable-valgrind --enable-debug
RUN make install
ENTRYPOINT ["/bin/bash", "-c", "./startup.sh" ]

