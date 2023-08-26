import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import * as fs from 'file-saver';

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
    const fileReader = new FileReader();
    

    fileReader.onload = (event) => {
      let binaryData = event.target?.result;
      alert(binaryData);
    }

    fileReader.onloadend = (event) => {
      console.log(fileReader.result);
    }

    fileReader.onerror = (event) => {
      console.log(event);
    }
    
    fileReader.readAsText(event.target.files[0]);
  }


}
