module.exports = {
  "env": {
    "node": true,
    "browser": true,
    "commonjs": true,
    "amd": true
  },
  extends: ["eslint:recommended"],
  parser: "@typescript-eslint/parser",
  parserOptions: {
    ecmaVersion: "latest",
    sourceType: "module",
  },
  plugins: ["@typescript-eslint"],
  rules: {
    "require-jsdoc": 0,
    "linebreak-style": ["off", "windows"],
    "quotes": ["error", "double"],
    "semi": ["error", "always"],
    // "no-undef": "off",
  },
};
