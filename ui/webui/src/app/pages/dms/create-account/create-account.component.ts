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

    this.http.createaccount(JSON.stringify(request)).subscribe((response: string) => {
      alert("Account Created Successfully");
      this.createaccountForm.reset('');
    },
    (error) => {},
    () => {});

  }
}
