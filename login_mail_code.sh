#!/bin/bash

login=1
tmp_file=/run/code.time
code=$(awk '{if(($2+3600)>systime())print$1}' $tmp_file 2>/dev/null)
[ -z "$code" ] && login=2

for i in `seq 3`; do
  if [ $login -eq 2 ];then
    code=$(awk 'BEGIN{srand();printf("%06d",1000000*rand())}')
    /janbar/sbin/sendmail xxx@163.com password yyy@qq.com login $code || exit $?
  fi
  if ! read -t 60 -s -p 'code: ' input;then
    exit 0
  fi
  echo

  if [ "$input" = "$code" ];then
    [ $login -eq 2 ] && echo "$code $(date +%s)" > $tmp_file
    login=0
    break
  fi
  echo 'Login incorrect'
done

if [ $login -eq 0 ];then
  /bin/bash
fi
