echo "starting up...."
echo "changing permissions"
chmod -R 777 /usr/local/var/ & ldconfig
dovecot 
echo "dovecot up and running"
tail -f /var/log/dovecot.log
