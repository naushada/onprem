import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';

@Component({
  selector: 'app-config',
  templateUrl: './config.component.html',
  styleUrls: ['./config.component.scss']
})
export class ConfigComponent {

  uploadTemplateForm: FormGroup;
  constructor(private fb: FormBuilder) {
    this.uploadTemplateForm = fb.group({
      templatename:'',
      devicemodel: ''
    });

  }

  onSubmit() {
    alert("I am invoked")

  }
  onChange(event:any) {

  }
}
