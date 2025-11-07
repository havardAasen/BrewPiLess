import js from "@eslint/js";
import globals from "globals";
import tseslint from "typescript-eslint";
import { defineConfig, globalIgnores } from "eslint/config";


export default defineConfig([
  globalIgnores(["dist/"]),
  {
    files: ["src/js/**/*.{js,mjs,cjs}"],
    plugins: { js }, extends: ["js/recommended"]
  },
  {
    files: ["src/js/**/*.{js,mjs,cjs}"],
    languageOptions: {
      globals: globals.browser
    }
  },

  {
    files: ["src/js/**/*.{ts,tsx}"],
    plugins: {
      "@typescript-eslint": tseslint.plugin,
    },
    languageOptions: {
      parser: tseslint.parser,
      parserOptions: {
        project: "./tsconfig.json",
      },
      globals: globals.browser,
    },
    rules: {
      ...tseslint.configs.recommended.rules,
    },
  },
]);
