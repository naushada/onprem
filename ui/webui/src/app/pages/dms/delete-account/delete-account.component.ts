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
    let request = {...this.deleteaccountForm.value};

    this.http.createaccount(JSON.stringify(request)).subscribe((response: string) => {
      alert("Account Created Successfully");
      this.deleteaccountForm.reset('');
    },
    (error) => {},
    () => {});

  }
}
