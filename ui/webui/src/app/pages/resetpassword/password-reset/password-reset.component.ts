import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { EventService } from 'src/common/event.service';

@Component({
  selector: 'app-password-reset',
  templateUrl: './password-reset.component.html',
  styleUrls: ['./password-reset.component.scss']
})
export class PasswordResetComponent {

  opened:boolean = false;

  passwordresetForm: FormGroup;
  constructor(private fb: FormBuilder, private event: EventService) {
    event.subscribe("password.reset", (id:string, document:string) => {
      if(id == "password.reset") {
        let response = JSON.parse(JSON.stringify(document)); 
        this.opened = response["state"];
        console.log("naushad: " + this.opened);
      }
    });

    this.passwordresetForm = fb.group({
      userid: '',
      password: '',
      confirmpassword:''
    });
  }

  onPasswordReset() {

  }
}
