#!/bin/bash

login=0
for i in `seq 3`; do
  if ! read -t 5 -p 'Username: ' username ;then
    exit 0
  fi

  if ! read -t 5 -s -p 'Password: ' password ;then
    exit 0
  fi
  echo

  diff <(echo $username:$password) /etc/janbar >/dev/null 2>&1
  if [ $? -eq 0 ];then
    login=1; break
  fi
  echo -e 'Login incorrect\n'
done

if [ $login -eq 1 ];then
  /bin/bash
fi
