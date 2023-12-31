import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { HttpService } from 'src/common/http.service';

@Component({
  selector: 'app-update-account',
  templateUrl: './update-account.component.html',
  styleUrls: ['./update-account.component.scss']
})
export class UpdateAccountComponent {
  
  updateaccountForm:FormGroup;
  selectedRole: string = "advanced";
  constructor(private fb: FormBuilder, private http: HttpService) {
    this.updateaccountForm = this.fb.group({
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
    let request = {...this.updateaccountForm.value};

    this.http.updateaccount(JSON.stringify(request)).subscribe((response: string) => {
      let responseobject = JSON.parse(JSON.stringify(response));
      if(responseobject["status"] == "success") {
        alert("Account Updated Successfully");
        this.updateaccountForm.reset('');
      } else {
        alert("Account Update Failed!");
      }
    },
    (error) => {},
    () => {});

  }
}
