#!/bin/bash

login=0
for i in $(seq 3); do
  code=$(awk 'BEGIN{srand();c[0]=42;c[1]=43;printf("%d %c %d",int(10*rand())+1,c[int(2*rand())],int(10*rand())+1)}')
  qrencode -t UTF8 -o - "$code = ?"

  if ! read -t 60 -s -p 'Password: ' password ;then
    exit 0
  fi
  echo

  cmp_str=$(cat /etc/janbar)$((code))
  if [ "${password,,}" = "${cmp_str,,}" ];then
    login=1; break
  fi
  echo -e 'Login incorrect\n'
done

if [ $login -eq 1 ];then
  /bin/bash
fi
