1. give rw permissions to read ceph conifguration

chmod 777 /etc/ceph/*

2. give read write access to dovecot mail direcotry

chmod -R 777 /usr/local/var/

3. build your plugin with make install
cd /repo

3.1 autogen files

 ./autogen.sh
3.2. configure the plugin

./configure --with-dovecot=/usr/local/lib/dovecot --enable-maintainer-mode --enable-debug --with-integration-tests --enable-valgrind --enable-debug

3.3 build and install the plugin 

make install 

4. reload all libraries 

ldconfig

5. start dovecot

dovecot

6. test save a mail

smtp-source -v -L -s 1 -m 1 -c -F /root/lmtp_test_mail.tld -f test@example.com -t t1 inet:127.0.0.1:1024 




