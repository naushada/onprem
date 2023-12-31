import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { HttpService } from 'src/common/http.service';

@Component({
  selector: 'app-delete-account',
  templateUrl: './delete-account.component.html',
  styleUrls: ['./delete-account.component.scss']
})
export class DeleteAccountComponent {

  deleteaccountForm:FormGroup;
  selectedRole: string = "advanced";
  constructor(private fb: FormBuilder, private http: HttpService) {
    this.deleteaccountForm = this.fb.group({
      userid: '',
    });
  }


  onSubmit() {

    this.http.deleteaccount(this.deleteaccountForm.value.userid).subscribe((response: string) => {
      let responseobject = JSON.parse(JSON.stringify(response));
      if(responseobject["status"] == "success") {
        alert("Account Deleted Successfully");
        this.deleteaccountForm.reset('');
      } else {
        alert("Account Deletion Failed!");
      }
    },
    (error) => {},
    () => {});

  }
}
