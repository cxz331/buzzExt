#!/bin/bash
./configure --enable-varstream
read -s -n1 -p "编译完成，准备make了，兄弟..."
make
read -s -n1 -p "make完成，就差把动态文件so复制到extensions目录了..."
cp ./modules/varstream.so /data/service/php-dev-5.5.28/lib/php/extensions/debug-zts-20121212/
echo -e '\nok!'
