import js from "@eslint/js";
import globals from "globals";
import tseslint from "typescript-eslint";
import { defineConfig, globalIgnores } from "eslint/config";
import eslintConfigPrettier from "eslint-config-prettier/flat";


export default defineConfig([
  globalIgnores(["dist/", "node_modules/"]),
  {
    languageOptions: {
      globals: {
        ...globals.browser
      },
    },
  },
  {
    files: ["src/js/**/*.{js,mjs,cjs}"],
    plugins: {
      js
    },
    extends: ["js/recommended"]
  },
  tseslint.configs.recommended,
  eslintConfigPrettier,
]);
