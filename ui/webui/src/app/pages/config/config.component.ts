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

  constructor(private fb: FormBuilder, private http: HttpService) {
    this.uploadTemplateForm = fb.group({
      templatename:'',
      devicemodel: ''
    });

  }

  onSubmit() {
    let request = {
      "filename": this.uploadTemplateForm.value.templatename,
      "productmodel": this.product,
      "fwversion": this.fwversion,
      "createdon": "",
      "content": this.template
    }

    this.http.createtemplate(JSON.stringify(request)).subscribe((response: string) => {
      let result = JSON.parse(JSON.stringify(response));
      if(result["status"] == "success") {
        alert("Template: " + this.uploadTemplateForm.value.templatename + " Released sucessfully");
      } else {
        alert("Template Release Failed");
      }
    },
    (error) => {},
    () => {});
  }
  onChange(event:any) {
    const fileReader = new FileReader();
    

    fileReader.onload = (event) => {
      let binaryData = event.target?.result;
      console.log(binaryData);
    }

    fileReader.onloadend = (event) => {
      //console.log(fileReader.result);
      this.template = JSON.stringify(fileReader.result);
      let result = JSON.parse(this.template);

      console.log(result);
      console.log(result["info.product"]);

      if(result["info.product"] == this.uploadTemplateForm.value.devicemodel) {
        if(this.template.length <= 16000000) {
          this.product = result["info.product"];
          this.fwversion = result["info.version"]
        } else {
          alert("Template size is > 16MB");
        }
      } else {
        alert("Template has product: " + result["info.product"] + 
              " selected device model: " + 
              this.uploadTemplateForm.value.devicemodel);
      }
    }

    fileReader.onerror = (event) => {
      console.log(event);
    }
    
    fileReader.readAsText(event.target.files[0]);
  }


}
