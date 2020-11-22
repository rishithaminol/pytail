#!/usr/bin/env python

import pytail

def callback_function(lines):
    print("given lines:", lines)

pytail.tail('./sample-tail.txt', callback_function)
