#!/bin/bash
serverFolder=~/openMerc/Source/Server

#Create folders to store data, if necessary
if [ ! -d ".obj" ]; then
    mkdir .obj
fi
if [ ! -d ".dat" ]; then
    mkdir .dat
fi

#install zlib (compression lib) and apache2 (server for world builder)
sudo apt-get install -y zlib1g-dev apache2

#the CGI scripts make use of the cgi-imp directory
#replace cgi-bin with cgi-imp
sudo sed -i 's/cgi-bin/cgi-imp/g' /etc/apache2/conf-available/serve-cgi-bin.conf

cd /etc/apache2/mods-enabled
#symbolic link to enable cgi
sudo ln -s ../mods-available/cgi.load
#reload the server
sudo service apache2 reload

#set permissions on our folders
# All read and execute for .cgi scripts
sudo chmod -R a+rx $serverFolder/cgi
#All read, write, and execute for .dat and .obj files,
#since the world builder modifies them
sudo chmod -R a+rwx $serverFolder/.dat
sudo chmod -R a+rwx $serverFolder/.obj

#make the cgi-imp folder, if necessary
if [ ! -d "/usr/lib/cgi-imp" ]; then
    sudo mkdir /usr/lib/cgi-imp
fi

cd /usr/lib/cgi-imp/

#copy the cgi files to it
sudo cp $serverFolder/cgi/acct.cgi .
sudo cp $serverFolder/cgi/info.cgi .

#make a symbolic link to the .dat and .obj folders
sudo ln -s $serverFolder/.dat
sudo ln -s $serverFolder/.obj
