#!/bin/bash
serverFolder=$(pwd)

#Create folders to store data, if necessary
if [ ! -d ".obj" ]; then
    mkdir .obj
fi
if [ ! -d ".dat" ]; then
    mkdir .dat
fi

#install zlib (compression lib) and apache2 (server for world builder)
sudo apt-get install -y zlib1g-dev apache2

sudo printf "<IfModule mod_alias.c>\n\t<IfModule mod_cgi.c>\n\t\tDefine ENABLE_USR_LIB_CGI_BIN\n\t</IfModule>\n\n\t<IfModule mod_cgid.c>\n\t\tDefine ENABLE_USR_LIB_CGI_BIN\n\t</IfModule>\n\n\t<IfDefine ENABLE_USR_LIB_CGI_BIN>\n\t\tScriptAlias /cgi-imp/ /usr/lib/cgi-imp/\n\t\t<Directory \"/usr/lib/cgi-imp\">\n\t\t\tAllowOverride None\n\t\t\tOptions +ExecCGI -MultiViews +SymLinksIfOwnerMatch\n\t\t\tRequire all granted\n\t\t</Directory>\n\t</IfDefine>\n\n\t<IfDefine ENABLE_USR_LIB_CGI_BIN>\n\t\tScriptAlias /cgi-bin/ /usr/lib/cgi-bin/\n\t\t<Directory \"/usr/lib/cgi-bin\">\n\t\t\tAllowOverride None\n\t\t\tOptions +ExecCGI -MultiViews +SymLinksIfOwnerMatch\n\t\t\tRequire all granted\n\t\t</Directory>\n\t</IfDefine>\n</IfModule>\n\n# vim: syntax=apache ts=4 sw=4 sts=4 sr noet" > /etc/apache2/conf-available/serve-cgi-bin.conf

cd /etc/apache2/mods-enabled
#symbolic link to enable cgi
sudo ln -s ../mods-available/cgi.load
#reload the server
sudo service apache2 reload

make -f Makefile || exit

#set permissions on our folders
# All read and execute for .cgi scripts
sudo chmod -R a+rx $serverFolder/cgi
#All read, write, and execute for .dat files,
#since the world builder modifies them
sudo chmod -R a+rwx $serverFolder/.dat


#make the cgi-imp folder, if necessary
if [ ! -d "/usr/lib/cgi-imp" ]; then
    sudo mkdir /usr/lib/cgi-imp
fi

cd /usr/lib/cgi-imp/

#copy the cgi files to it
sudo cp $serverFolder/cgi/acct.cgi .
#make a symbolic link to the .dat folder
sudo ln -s $serverFolder/.dat

cd /usr/lib/cgi-bin/
#copy the cgi files to it
sudo cp $serverFolder/cgi/info.cgi .
#make a symbolic link to the .dat folder
sudo ln -s $serverFolder/.dat

sudo mkdir /var/www/html/gfx
sudo mv $serverFolder/gfx/*.gif /var/www/html/gfx
