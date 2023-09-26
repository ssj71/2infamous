```
::::::::::: ::::    ::: ::::::::::   :::     ::::    ::::   ::::::::  :::    :::  ::::::::  
    :+:     :+:+:   :+: :+:        :+: :+:   +:+:+: :+:+:+ :+:    :+: :+:    :+: :+:    :+: 
    +:+     :+:+:+  +:+ +:+       +:+   +:+  +:+ +:+:+ +:+ +:+    +:+ +:+    +:+ +:+        
    +#+     +#+ +:+ +#+ :#::+::# +#++:++#++: +#+  +:+  +#+ +#+    +:+ +#+    +:+ +#++:++#++ 
    +#+     +#+  +#+#+# +#+      +#+     +#+ +#+       +#+ +#+    +#+ +#+    +#+        +#+ 
    #+#     #+#   #+#+# #+#      #+#     #+# #+#       #+# #+#    #+# #+#    #+# #+#    #+# 
########### ###    #### ###      ###     ### ###       ###  ########   ########   ########  
                                                                                  plugins.
```
v. 0.1

This is a new collection of lv2 audio plugins for linux.
I've started another collection to support a new build system.
If you are looking for the other (older) infamous plugins go to https://github.com/ssj71/infamousplugins

These do not replace those.


If you want to build these run the following commands
```
mkdir build
cd build
meson ..
ninja
sudo ninja install
```

The dependencies are fairly minimal. Mostly the meson and ninja build systems and the package `lv2-dev` on debian or ubuntu.

Currently this collection has the following plugins:
 * the infamous bent delay mk II - a(nother) delay plugin, this one randomly modulates the delay time and amplitude for various effects
 * the infamous atari punk console - noise generator toy based on a popular DIY circuit
 * the infamous OK saturation - the first of the OK series, a one knob saturation effect based very loosely on tube saturation dynamics
