import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { EventService } from 'src/common/event.service';
import { HttpService } from 'src/common/http.service';

@Component({
  selector: 'app-password-reset',
  templateUrl: './password-reset.component.html',
  styleUrls: ['./password-reset.component.scss']
})
export class PasswordResetComponent {

  opened:boolean = false;

  passwordresetForm: FormGroup;
  constructor(private fb: FormBuilder, private event: EventService, private http: HttpService) {
    event.subscribe("password.reset", (id:string, document:string) => {
      if(id == "password.reset") {
        let response = JSON.parse(document); 
        this.opened = response["state"] as boolean;
      }
    });

    this.passwordresetForm = fb.group({
      userid: '',
      password: '',
      confirmpassword:''
    });
  }

  onPasswordReset() {

    if(this.passwordresetForm.value.password != this.passwordresetForm.value.confirmpassword) {
      alert("password and confirmpassword is not same!");
      return;
    }
    
    let request = {
      "userid": this.passwordresetForm.value.userid,
      "password": this.passwordresetForm.value.password
    };

    this.http.updateaccount(JSON.stringify(request)).subscribe((response: string) => {
      this.opened = false;
      let responseobject = JSON.parse(response);
      if(responseobject["status"] == "success") {
        alert("Password is set successfully")
      } else {
        alert("Password set is failed");
      }
    },
    (error) => {},
    () => {});
  }
}
