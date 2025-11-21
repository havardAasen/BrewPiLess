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
  {
    files: ["src/js/**/*.{ts,tsx}"],
    extends: [tseslint.configs.recommended, tseslint.configs.stylistic],
    plugins: {
      "@typescript-eslint": tseslint.plugin,
    },
    languageOptions: {
      parser: tseslint.parser,
      parserOptions: {},
    },
    rules: {
      '@typescript-eslint/no-explicit-any': 'off',
    },
  },
  eslintConfigPrettier,
]);
