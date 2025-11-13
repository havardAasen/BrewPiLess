'use strict';

module.exports = function(grunt) {
  grunt.loadNpmTasks('grunt-contrib-compress');
  grunt.loadNpmTasks('grunt-shell');
  grunt.loadNpmTasks('grunt-contrib-htmlmin');
  grunt.loadNpmTasks('grunt-contrib-sass');
  grunt.loadNpmTasks('grunt-replace');
  grunt.loadNpmTasks('grunt-contrib-watch');
  grunt.loadNpmTasks('grunt-multi-lang-site-generator');
  grunt.loadNpmTasks('grunt-processhtml');

  const languages =  ['english', 'chinese', 'spanish', 'portuguese-br', 'slovak']
  const translations = {}

  languages.forEach(language => {
    translations[language] = grunt.file.readJSON(`src/locales/${language}.json`)
  })

  const replaceConfig = {}
  languages.forEach(language => {
    const patterns = Object.entries(translations[language])
    .map(([key, value]) => ({
      match: new RegExp(`<%=\\s*${key}\\s*%>`, 'g'),
      replacement: value,
    }))

    replaceConfig[language] = {
      options: {
        patterns,
        silent: true
      },
      files: [
        {
          expand: true,
          cwd: 'dist',
          src: ['bundle.js'],
          dest: `dist/${language}/`
        }
      ]
    }
  })

  grunt.initConfig({

    shell: {
      debug: {
        command: 'NODE_ENV=development rollup -c'
      },
      prod: {
        command: 'NODE_ENV=production rollup -c'
      }
    },

    htmlmin: {
      dist: {
        options: {
          removeComments: true,
          collapseWhitespace: true,
          collapseBooleanAttributes: true,
          removeAttributeQuotes: true,
          removeRedundantAttributes: false,
          removeEmptyAttributes: true,
          minifyJS: true,
          minifyCSS: true
        },
        files: {
          'dist/index.tmpl.htm': 'dist/index.tmpl.htm',
          'dist/control.tmpl.htm': 'dist/control.tmpl.htm',
          'dist/setup.tmpl.htm': 'dist/setup.tmpl.htm',
          'dist/gravity.tmpl.htm': 'dist/gravity.tmpl.htm',
          'dist/logging.tmpl.htm': 'dist/logging.tmpl.htm',
          'dist/config.tmpl.htm': 'dist/config.tmpl.htm',
          'dist/pressure.tmpl.htm': 'dist/pressure.tmpl.htm',
          'dist/BPLLogViewer.tmpl.htm': 'dist/BPLLogViewer.tmpl.htm',
          'dist/BPLog.tmpl.htm': 'dist/BPLog.tmpl.htm',
          'dist/lcd.htm': 'dist/lcd.htm',
          'dist/testcmd.htm': 'dist/testcmd.htm',
          'dist/edit.htm': 'dist/edit.htm'
        }
      }
    },

    sass: {
      dev: {
        options: {
          style: 'expanded',
          noSourceMap: true
        },
        files: [
          { expand: true, cwd: 'src/styles/', src: ['*.scss'], dest: './build/', ext: '.css' }
        ]
      }
    },

    compress: {
      main: {
        options: {
          mode: 'gzip',
          level: 9
        },
        files: [
          {
            expand: true,
            src: [
              'dist/*/*.htm',
              'dist/lcd.htm',
              'dist/testcmd.htm',
              'dist/edit.htm'
            ],
            dest: '.',
            ext: '.htm.gz'
          },
          {
            expand: true,
            src: ['dist/**/*.js'],
            dest: '.',
            ext: '.js'  // keep the same name for consistent HTML reference
          }
        ]
      }
    },

    processhtml: {
      dist: {
        files: {
          'dist/index.tmpl.htm': ['src/index.tmpl.html'],
          'dist/control.tmpl.htm': ['src/control.tmpl.html'],
          'dist/setup.tmpl.htm': ['src/setup.tmpl.html'],
          'dist/gravity.tmpl.htm': ['src/gravity.tmpl.html'],
          'dist/logging.tmpl.htm': ['src/logging.tmpl.html'],
          'dist/config.tmpl.htm': ['src/config.tmpl.html'],
          'dist/pressure.tmpl.htm': ['src/pressure.tmpl.html'],
          'dist/BPLLogViewer.tmpl.htm': ['src/BPLLogViewer.tmpl.html'],
          'dist/BPLog.tmpl.htm': ['src/BPLog.tmpl.html'],
          'dist/lcd.htm': ['src/lcd.html'],
          'dist/testcmd.htm': ['src/testcmd.html'],
          'dist/edit.htm': ['src/edit.html']
        }
      }
    },

    multi_lang_site_generator: {
      default: {
        options: {
          vocabs: languages,
          vocab_directory: 'src/locales',
          output_directory: 'dist',
          template_directory: 'dist'
        },
        files: [
          {'index.htm': ['index.tmpl.htm']},
          {'control.htm': ['control.tmpl.htm']},
          {'setup.htm': ['setup.tmpl.htm']},
          {'gravity.htm': ['gravity.tmpl.htm']},
          {'logging.htm': ['logging.tmpl.htm']},
          {'config.htm': ['config.tmpl.htm']},
          {'pressure.htm': ['pressure.tmpl.htm']},
          {'BPLog.htm': ['BPLog.tmpl.htm']},
          {'BPLLogViewer.htm': ['BPLLogViewer.tmpl.htm']}
        ]
      }
    },

    replace: replaceConfig,

    watch: {
      files: ['src/**/*'],
      tasks: ['default']
    }
  });

  const multi_lang_js_gen = languages.map(lang => `replace:${lang}`)

  grunt.registerTask('debug', [
    'shell:debug',
    'processhtml',
    'sass',
    'multi_lang_site_generator',
      ...multi_lang_js_gen,
    'compress'
  ]);

  grunt.registerTask('default', [
    'shell:debug',
    'processhtml',
    'sass',
    'multi_lang_site_generator',
      ...multi_lang_js_gen,
    'watch'
  ]);

  grunt.registerTask('build', [
    'shell:prod',
    'processhtml',
    'sass',
    'htmlmin',
    'multi_lang_site_generator',
    ...multi_lang_js_gen,
    'compress'
  ]);
};
