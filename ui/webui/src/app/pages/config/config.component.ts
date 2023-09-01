import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import * as fs from 'file-saver';
import { HttpService } from 'src/common/http.service';

@Component({
  selector: 'app-config',
  templateUrl: './config.component.html',
  styleUrls: ['./config.component.scss']
})
export class ConfigComponent {

  uploadTemplateForm: FormGroup;
  template:string = "";
  product:string = "";
  fwversion:string = "";
  filename:string = "";
  isEnabled:boolean = true;
  constructor(private fb: FormBuilder, private http: HttpService) {
    this.uploadTemplateForm = fb.group({
      templatename:''
    });

  }

  onSubmit() {
    let filepath = this.uploadTemplateForm.value.templatename;
    this.filename = filepath.substring(filepath.lastIndexOf('\\') + 1 );

    if(this.filename == filepath) {
      this.filename = filepath.substring(filepath.lastIndexOf('/') + 1 );
    }

    let request = {
      "filename": this.filename,
      "productmodel": this.product,
      "fwversion": this.fwversion,
      "createdon": new Date(),
      "content": this.template
    }

    this.http.createtemplate(JSON.stringify(request)).subscribe((response: string) => {
      let result = JSON.parse(JSON.stringify(response));
      if(result["status"] == "success") {
        alert("Template: " + this.filename + " Released sucessfully");
      } else {
        alert("Template Release Failed");
      }
    },
    (error) => {},
    () => {});
  }

  onChange(event:any) {

    if(!this.uploadTemplateForm.value.templatename.length) {
      alert("Please select the Template File");
      return;
    }

    this.filename = event.target.files[0];

    const fileReader = new FileReader();
    fileReader.onload = (event) => {
      let binaryData = event.target?.result;
      console.log(binaryData);
    }

    fileReader.onloadend = (event) => {
      //console.log(fileReader.result);
      this.template = btoa(fileReader.result as string);
      let result = JSON.parse(fileReader.result as string);

      let info = JSON.parse(JSON.stringify(result["info"]));

      if(info["product"] != null && this.template.length <= 16000000) {
          this.product = info["product"];
          this.fwversion = info["version"];
          this.isEnabled = false;
          
      } else {
        alert("Either Product info or template size is > 16 MB");
      }
    }

    fileReader.onerror = (event) => {
      console.log(event);
    }
    
    fileReader.readAsText(event.target.files[0]);
  }


}
