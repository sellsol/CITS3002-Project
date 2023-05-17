/**
 * Submits the login form.
 * Retrieves the username and password from the form inputs and makes an AJAX call to authenticate the user's credentials.
 */
function submitLoginForm() {
  const username = document.getElementById("username").value;
  const password = document.getElementById("password").value;

  // Make an AJAX call to authenticate the user's credentials
  const xhr = new XMLHttpRequest();
  xhr.open("POST", "/login", true);
  xhr.setRequestHeader("Content-Type", "application/json");
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      console.log(xhr);
      // Successful request
      if (xhr.status === 200) {
        console.log("200");
        window.location.href = "/";
      // Invalid credentials
      } else if (xhr.status === 401) {
        console.log("401");
        alert("Invalid username or password.");
      }
    }
  };
  const data = {
    username: username,
    password: password,
  };
  console.log(data);
  xhr.send(JSON.stringify(data));
}

/**
 * Listens for the form submission event and prevents the default form submission behavior.
 * Calls the submitLoginForm function to handle the form submission.
 */
document
  .getElementById("login-form")
  .addEventListener("submit", function (event) {
    event.preventDefault();
    submitLoginForm();
  });
