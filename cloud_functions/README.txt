HOW TO COMPILE FUNCTIONS TO FIREBASE:
new project:
    1. ceate a directory:
        * cd to this directory and run firebase init
        * choose functions when asked
        * don't choose the tslint option (cancer)
compile function:
    cd to the functions folder then run "npm run-script build"
deploy localy:
    cd to functions folder and then run "firebase serve --only functions"
deploy on server:
    cd to function folder and then run "firebase deploy"