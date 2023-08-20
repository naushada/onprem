import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { Router } from '@angular/router';
import { EventService } from 'src/common/event.service';
import { HttpService } from 'src/common/http.service';

@Component({
  selector: 'app-login',
  templateUrl: './login.component.html',
  styleUrls: ['./login.component.scss']
})
export class LoginComponent {

  openpasswordreset:boolean = false;
  loginForm: FormGroup
  constructor(private fb: FormBuilder, private rt:Router, private http: HttpService, private event: EventService) {
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
    if(!this.username.length || !this.password.length) {
      alert("User ID or Password Can't be Empty");
    }
    
    this.http.getlogininfo(this.username, this.password).subscribe((accountinfo:string) => {
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
    
  }

  onPasswordReset() {
    this.openpasswordreset = true;
    let id: string = "password.reset";
    let document: string = "{\"state\": " + "true}";
    this.event.publish({id, document});
  }
}
