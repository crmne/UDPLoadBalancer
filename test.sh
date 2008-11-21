#!/bin/bash
urxvt -e test/Monitor.exe &
sleep 1
urxvt -e ./MobileLoadBalancer &
urxvt -e ./FixedLoadBalancer &
sleep 1
urxvt -e test/Appfixed.exe &
urxvt -e test/Appmobile.exe
