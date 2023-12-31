import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { HttpService } from 'src/common/http.service';

@Component({
  selector: 'app-create-account',
  templateUrl: './create-account.component.html',
  styleUrls: ['./create-account.component.scss']
})
export class CreateAccountComponent {

  createaccountForm:FormGroup
  selectedRole: string = "advanced";
  constructor(private fb: FormBuilder, private http: HttpService) {
    this.createaccountForm = this.fb.group({
      userid: '',
      password:'',
      username:'',
      role: ''
    });
  }


  onSelect(event:any, role:string) {
    if(event.target.checked) {
      //this.createaccountForm.value.role.push(role);
      this.selectedRole = role;
    }
  }

  onSubmit() {
    let request = {...this.createaccountForm.value};
    //alert("The request: " + request);
    //console.log(request);
    this.http.createaccount(JSON.stringify(request)).subscribe((response: string) => {
      let responseobject = JSON.parse(JSON.stringify(response));
      if(responseobject["status"] == "success") {
        alert("Account Created Successfully");
        this.createaccountForm.reset('');
      } else {
        alert("Account Creation Failed!");
      }
    },
    (error) => {},
    () => {});

  }
}
