// Initialise Variables
var currentQuestion = 0;
var questions = [""];
var types = [""]; //type can be "c" for coding or "m" for multiple choice
var choices = [""];
var username = "";
var current_finished = 0;
var current_marks = 0;
var attempts = [];
var marks = [];
var displayImage = false;

// Adds things to screen
function displayQuestion() {
  document.getElementById("question-number").innerHTML = currentQuestion + 1;
  document.getElementById("question").innerHTML = questions[currentQuestion];
  document.getElementById("username-header").innerHTML = username;
  document.getElementById("current-finished").innerHTML = current_finished;
  document.getElementById("current-marks").innerHTML = current_marks;
  document.getElementById("attempts").innerHTML = attempts[currentQuestion];
  document.getElementById("marks").innerHTML = marks[currentQuestion];
  document.getElementById("current-finished-out-of").innerHTML =
    questions.length;
  document.getElementById("current-marks-out-of").innerHTML =
    3 * questions.length;
  document.getElementById("student-output-image").style.display = displayImage
    ? "block"
    : "none";
  document.getElementById("sample-output-image").style.display = displayImage
    ? "block"
    : "none";
  if (types[currentQuestion] == "c") {
    document.getElementById("multichoice-screen").style.display = "none";
    document.getElementById("coding-screen").style.display = "block";
    document.getElementById("answer").value = "";
  }
  if (types[currentQuestion] == "m") {
    document.getElementById("multichoice-screen").style.display = "block";
    document.getElementById("coding-screen").style.display = "none";
    document.getElementById("answer").value = "";
    options = document.getElementById("options");

    while (options.firstChild) {
      options.removeChild(options.firstChild);
    }

    for (let i = 0; i < choices[currentQuestion].length; i++) {
      const div = document.createElement("div");
      div.classList.add("mcq-choice");
      const radio = document.createElement("input");
      radio.type = "radio";
      radio.name = "mcq";
      radio.value = i;
      const label = document.createElement("label");
      label.textContent = choices[currentQuestion][i];
      div.appendChild(radio);
      div.appendChild(label);
      options.appendChild(div);
    }
  }
}

// Sends answer from form to python
function submitAnswer() {
  if (attempts[currentQuestion] > 2 || marks[currentQuestion] != 0) return;
  var answer;
  if (types[currentQuestion] == "c") {
    answer = document.getElementById("answer").value;
  } else if (types[currentQuestion] == "m") {
    event.preventDefault();
    answer = document.querySelector('input[name="mcq"]:checked').value;
  } else {
    return;
  }

  console.log("Answer: " + answer);

  // make an HTTP POST request to the Python server with the form data
  var xhr = new XMLHttpRequest();
  xhr.open("POST", "/submit-answer", true);
  xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      if (xhr.status === 200) {
        try {
          var response = JSON.parse(xhr.responseText);
          console.log(response); // Output the parsed response
        } catch (error) {
          console.log(xhr);
          console.error("Error parsing JSON:", error);
        }
        if (response.correct) {
          correct();
        } else {
          const studentOutput =
            response.student_output != null ? response.student_output : "";
          const sampleOutput =
           response.sample_output != null ? response.sample_output : "";
          if (response.image) {
            incorrectImage(studentOutput, sampleOutput);
          } else {
            incorrect(studentOutput, sampleOutput);
          }
        }

        // Handle the response from the server here
        displayQuestion();
        console.log(xhr.responseText);
      } else {
        // Handle errors here
        console.error(xhr.statusText);
      }
    }
  };

  xhr.send(
    username +
      ":" +
      encodeURIComponent(answer) +
      ":" +
      currentQuestion +
      ":" +
      attempts[currentQuestion]
  );
}

// Change to previous question
function previousQuestion() {
  if (currentQuestion > 0) {
    document.getElementById("sample-output").textContent = "";
    document.getElementById("student-output").textContent = "";
    displayImage = false;
    currentQuestion--;
    displayQuestion();
  }
}

// Change to next question
function nextQuestion() {
  if (currentQuestion < questions.length - 1) {
    document.getElementById("sample-output").textContent = "";
    document.getElementById("student-output").textContent = "";
    displayImage = false;
    currentQuestion++;
    displayQuestion();
  }
}

// Updates for correct answers
function correct() {
  marks[currentQuestion] = 3 - attempts[currentQuestion];
  attempts[currentQuestion]++;
  current_finished++;
  current_marks += marks[currentQuestion];
}

// Updates for incorrect answers
function incorrect(studentOutput, sampleOutput) {
  document.getElementById("student-output").innerHTML =
    studentOutput != "" ? "Your Output: <br>" + studentOutput : "";
  document.getElementById("sample-output").innerHTML =
    sampleOutput != "" ? "Expected Output: <br>" + sampleOutput : "";
  attempts[currentQuestion]++;
  if (attempts[currentQuestion] == 3) current_finished++;
}

function incorrectImage(studentOutput, sampleOutput) {
  document.getElementById("student-output").innerHTML =
    studentOutput != "" ? "Your Output: <br>" : "";
  document.getElementById("sample-output").innerHTML =
    sampleOutput != "" ? "Expected Output: <br>" : "";
  document.getElementById("student-output-image").src =
    "data:image/png;base64," + studentOutput;
  document.getElementById("sample-output-image").src =
    "data:image/png;base64," + sampleOutput;
  displayImage = true;
  attempts[currentQuestion]++;
  if (attempts[currentQuestion] == 3) current_finished++;
}

// Resize answer box
const answer = document.getElementById("answer");
answer.addEventListener("input", () => {
  answer.style.height = "auto"; /* reset the height */
  answer.style.height =
    answer.scrollHeight + "px"; /* adjust the height to fit the content */
});

// Display page when page is loaded
displayQuestion();
// Adds listener for submitting answer
document
  .getElementById("answer-form")
  .addEventListener("submit", function (event) {
    event.preventDefault();
    submitAnswer();
  });

// Logs out by deleting cookie and asking to remake page
function logout() {
  var cookies = document.cookie.split(";");

  for (var i = 0; i < cookies.length; i++) {
    var cookie = cookies[i];
    var eqPos = cookie.indexOf("=");
    var name = eqPos > -1 ? cookie.substr(0, eqPos) : cookie;
    document.cookie = name + "=;expires=Thu, 01 Jan 1970 00:00:01 GMT;";
  }
  window.location.href = "/";
}

// make an AJAX call to authenticate the user's credentials
var xhr = new XMLHttpRequest();
xhr.open("POST", "/get-data", true);
xhr.setRequestHeader("Content-Type", "application/json");
xhr.onreadystatechange = function () {
  if (xhr.readyState === XMLHttpRequest.DONE && xhr.status === 200) {
    var response = JSON.parse(xhr.responseText);
    console.log(response);
    if (response.success) {
      // authentication succeeded, display the question screen
      username = response.username;
      questions = response.questions;
      types = response.types;
      choices = response.choices;
      current_finished = response.current_finished;
      current_marks = response.current_marks;
      attempts = response.attempts;
      marks = response.marks;

      console.log(questions);
      console.log(types);
      console.log(choices);
      console.log(current_finished);
      console.log(current_marks);
      console.log(attempts);
      console.log(marks);
      displayQuestion();
    } else {
      // authentication failed, display an error message
      alert("Invalid username or password.");
    }
  }
};
xhr.send(JSON.stringify({ key: "value" }));

// Prevents default tab functionality
document.getElementById("answer").addEventListener("keydown", function (event) {
  if (event.key === "Tab") {
    event.preventDefault(); // Prevent the default tab behavior

    // Get the current text box value and cursor position
    var textbox = event.target;
    var value = textbox.value;
    var start = textbox.selectionStart;
    var end = textbox.selectionEnd;

    // Insert a tab character at the cursor position
    textbox.value = value.substring(0, start) + "\t" + value.substring(end);

    // Set the cursor position after the inserted tab
    textbox.selectionStart = textbox.selectionEnd = start + 1;
  }
});
