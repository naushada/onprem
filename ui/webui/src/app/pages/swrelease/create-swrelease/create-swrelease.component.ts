import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { HttpService } from 'src/common/http.service';
//import { extract, pack, on } from 'tar-fs';
//import { fs } from 'fs';
import * as JSZip from 'jszip';
import { saveAs } from 'file-saver';

@Component({
  selector: 'app-create-swrelease',
  templateUrl: './create-swrelease.component.html',
  styleUrls: ['./create-swrelease.component.scss']
})
export class CreateSwreleaseComponent {

  template:string = "";
  product:string = "";
  fwversion:string = "";
  filename:string = "";
  isEnabled:boolean = true;

  createSwReleaseForm: FormGroup;
  constructor(private fb: FormBuilder, private http: HttpService) {
    this.createSwReleaseForm = fb.group({
      fwreleasename:''
    });
  }

  async extractZip(file: File) {
    const zip = new JSZip();
    const extractedFiles = await zip.loadAsync(file);
    extractedFiles.forEach(async (relativePath, file) => {
      const content = await file.async("string");
      saveAs(content, relativePath);
    });
  }
  
  onSubmit() {
    let filepath = this.createSwReleaseForm.value.fwreleasename;
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

    this.http.uploadsoftwarerelease(JSON.stringify(request)).subscribe((response: string) => {
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
    if(!this.createSwReleaseForm.value.fwreleasename.length) {
      alert("Please select the FW Package File");
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
