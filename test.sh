#!/bin/bash
urxvt -e test/Monitor.exe &
sleep 2
urxvt -e ./MobileLoadBalancer &
sleep 1
urxvt -e ./FixedLoadBalancer &
sleep 2
urxvt -e test/Appfixed.exe &
sleep 1
urxvt -e test/Appmobile.exe
