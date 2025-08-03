var Module = {
  onRuntimeInitialized: function () {
    Module.ccall("tyson_init");

    document.getElementById("run").onclick = () => {
      console.log("Running!");
      const code = document.getElementById("input").value;
      const result = Module.ccall(
        "eval_string",
        "string",
        ["string"],
        [code]
      );
      document.getElementById("output").textContent += `> ${code}\n${result}\n`;
    };
  }
};
