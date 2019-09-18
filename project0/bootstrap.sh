#!/usr/bin/env bash

dir=/home/vagrant
apt-get update
apt-get install -y git vim cgdb samba wget
sudo mv ${dir}/smb.conf /etc/samba/smb.conf
sudo smbpasswd -a vagrant < smbuser
rm smbuser
sudo service smbd restart

# Vim configuration
mkdir -p ${dir}/.vim/colors
mv ${dir}/dracula.vim ${dir}/.vim/colors/
