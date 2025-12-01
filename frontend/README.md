# New Frontend for BPL

If you just want to test, try or use the new layout, please download the dist/ folder and upload the files to your BPL. If you are running the latest version with gzip-support, upload the .gz-files, otherwise the .htm-files

## Developing

For developing / building the project you need to have `nodejs` installed on your system.

Clone the repository and either use the development container, or run the following commands in the frontend/ folder to install the necessary dependencies:

``` bash
npm install
npm install -g grunt-cli
```

### Watch Mode

We use grunt to automate development and building tasks. If you want to test your changes locally, run grunt in watch mode (default) using:

``` bash
grunt
```

This will automatically recompile the code, you will just need to refresh the browser when it is finished.

### Build

If you want to build the frontend:

``` bash
npm run build
```
