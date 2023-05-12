# autoconf-skel
autoconf skeleton for my projects

## Usage
```bash
git clone https://github.com/Kara1147/autoconf-skel.git myrepo-name
cd myrepo-name
rm -rf .git
git init
```

## Library requirements
*none*

## Additional Information
This appliction is set up to be built using autotools. You may use
`autoreconf --install` to produce `./configure`. It's a good idea to use
`./configure` in a `build/` subdirectory or outside of the build tree.

Feel free to propose changes to the functionality or documentation of this
application.

