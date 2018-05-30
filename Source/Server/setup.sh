#!/bin/bash
currentFolder=$(pwd)

#Create folders to store data, if necessary
if [ ! -d ".obj" ]; then
    mkdir .obj
fi

#install zlib (compression lib) and apache2 (server for world builder)
sudo apt-get install -y zlib1g-dev gcc make apache2

printf "<IfModule mod_alias.c>\n\t<IfModule mod_cgi.c>\n\t\tDefine ENABLE_USR_LIB_CGI_BIN\n\t</IfModule>\n\n\t<IfModule mod_cgid.c>\n\t\tDefine ENABLE_USR_LIB_CGI_BIN\n\t</IfModule>\n\n\t<IfDefine ENABLE_USR_LIB_CGI_BIN>\n\t\tScriptAlias /cgi-imp/ /usr/lib/cgi-imp/\n\t\t<Directory \"/usr/lib/cgi-imp\">\n\t\t\tAllowOverride None\n\t\t\tOptions +ExecCGI -MultiViews +SymLinksIfOwnerMatch\n\t\t\tRequire all granted\n\t\t</Directory>\n\t</IfDefine>\n\n\t<IfDefine ENABLE_USR_LIB_CGI_BIN>\n\t\tScriptAlias /cgi-bin/ /usr/lib/cgi-bin/\n\t\t<Directory \"/usr/lib/cgi-bin\">\n\t\t\tAllowOverride None\n\t\t\tOptions +ExecCGI -MultiViews +SymLinksIfOwnerMatch\n\t\t\tRequire all granted\n\t\t</Directory>\n\t</IfDefine>\n</IfModule>\n\n# vim: syntax=apache ts=4 sw=4 sts=4 sr noet" > serve-cgi-bin.conf

sudo mv serve-cgi-bin.conf /etc/apache2/conf-available/serve-cgi-bin.conf

cd /etc/apache2/mods-enabled
#symbolic link to enable cgi
sudo ln -s ../mods-available/cgi.load
#reload the server
sudo service apache2 reload

cd $currentFolder

sudo make -f Makefile || error
