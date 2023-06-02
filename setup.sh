chmod 777 /etc/ceph/*
ceph tell mon.\* injectargs "--mon-allow-pool-delete=true"
chmod -R 777 /usr/local/var/
ldconfig 