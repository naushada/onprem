import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { Router } from '@angular/router';

@Component({
  selector: 'app-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.scss']
})
export class LoginComponent {

  loginForm: FormGroup
  constructor(private fb: FormBuilder, private rt:Router) {
    this.loginForm = this.fb.group({
      username: '',
      password: ''
    });
  }

  get username() {
    return this.loginForm.value.username;
  }

  get password() {
    return this.loginForm.value.password;
  }

  onLogin() {

    this.rt.navigateByUrl('/main');

    if(!this.username.length || !this.password.length) {
      alert("User ID or Password Can't be Empty");
    }
    /*
    this.http.getlogininfo(this.username, this.password).subscribe((accountinfo:IAccountInfo) => {
      let id:string = "user.login";
      let document: string = JSON.stringify(accountinfo);
      this.event.publish({id, document});
      this.rt.navigateByUrl('/main');
    },
    (error) => {
      alert("Invalid Credentials");
    },
    () => {

    });
    */
  }
}
