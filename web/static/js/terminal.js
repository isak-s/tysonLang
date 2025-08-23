function handleKeypress(e, input, output) {
  function noInputHasFocus() {
    const elements = ['INPUT', 'TEXTAREA', 'BUTTON']
    return elements.indexOf(document.activeElement.tagName) === -1
  }
  
  if (noInputHasFocus()) {
    // Prevent default behavior for keys we handle
    if (e.key === 'Enter' || e.key === 'Backspace' || (e.key.length === 1 && !e.ctrlKey && !e.altKey && !e.metaKey)) {
      e.preventDefault()
    }
    
    if (e.key === 'Enter') {
      // Get text content without the cursor
      const cursor = document.getElementById('cursor')
      cursor.remove()
      const command = input.innerText.trim()
      
      // Clear input and restore cursor
      input.innerHTML = ''
      input.appendChild(cursor)
      
      output.innerHTML += `<br><strong>&gt; ${command}</strong>\n<br>`
      execute(command).then((result) => {
        writeText(output, result)
      })
    } else if (e.key === 'Backspace') {
      // Remove cursor, delete last character, restore cursor
      const cursor = document.getElementById('cursor')
      cursor.remove()
      const currentText = input.innerText
      if (currentText.length > 0) {
        input.innerText = currentText.slice(0, -1)
      }
      input.appendChild(cursor)
    } else if (e.key.length === 1 && !e.ctrlKey && !e.altKey && !e.metaKey) {
      // Only add printable characters (not modifier keys, function keys, etc.)
      const cursor = document.getElementById('cursor')
      cursor.remove()
      input.insertAdjacentText('beforeend', e.key)
      input.appendChild(cursor)
    }
  }
  
  async function execute(command) {
    switch (command.toLowerCase()) {
      case '':
        return '\n'
      case 'clear':
        // Fixed: match the actual HTML id (lowercase)
        document.getElementById('asciitext').style.display = 'none'
        output.innerHTML = ''
        return ''
      case 'help':
        return `Commands:
  help  - show this message
  clear - clear the screen
  std - print entire standard library
  (or enter any valid TysonLang expression)`
      case 'std':
        return FS.readFile('std.tyson', { encoding: 'utf8' }); // contents of std.tyson
      default:
        if (typeof Module !== 'undefined' && Module.ccall) {
          try {
            const result = Module.ccall(
              'eval_string',
              'string',
              ['string'],
              [command]
            )
            return result
          } catch (e) {
            return `Error: ${e.message}`
          }
        } else {
          return 'Interpreter not ready yet.'
        }
    }
  }
}

const wait = (ms = 0) => new Promise(resolve => setTimeout(resolve, ms))

// Write text to a target element with a specified delay in ms
function writeText(target, content, delay = 5) {
  // Loop through array of content characters
  return new Promise((resolve) => {
    // Make an array of the specified content
    const contentArray = content.split('')
    // Keep track of the character currently being written
    let current = 0
    while (current < contentArray.length) {
      ;((curr) => {
        setTimeout(() => {
          target.innerHTML += contentArray[curr]
          // Scroll to the bottom of the screen unless scroll is false
          window.scrollTo(0, document.body.scrollHeight)
          
          // Resolve the promise once the last character is written
          if (curr === contentArray.length - 1) resolve()
        }, delay * curr) // increase delay with each iteration
      })(current++)
    }
  })
}

document.addEventListener('DOMContentLoaded', () => {
  // Fixed: match the actual HTML id (lowercase)
  const asciiText = document.getElementById('asciitext')
  const asciiArt = asciiText.innerText
  asciiText.innerHTML = ''
  const instructions = document.getElementById('instructions')
  const prompt = document.getElementById('prompt')
  const cursor = document.getElementById('cursor')
  const input = document.getElementById('command-input')
  const output = document.getElementById('output')
  
  if (typeof Module !== 'undefined') {
    Module.onRuntimeInitialized = async () => {
      Module.ccall("tyson_init") // init interpreter
      await wait(500)
      await writeText(asciiText, asciiArt)
      await wait(500)
      await writeText(instructions, `Enter a command. Type 'help' for options.`)
      cursor.innerHTML = '_' // show the blinking underscore
      document.addEventListener('keydown', (e) => handleKeypress(e, input, output))
    }
  } else {
    console.error("Module not defined. Is tyson.js loading correctly?")
  }
})