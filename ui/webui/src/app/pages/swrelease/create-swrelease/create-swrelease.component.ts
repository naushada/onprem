import { Component } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { HttpService } from 'src/common/http.service';
//import { extract, pack, on } from 'tar-fs';
//import { fs } from 'fs';
import * as JSZip from 'jszip';
import xml2js from 'xml2js';  


@Component({
  selector: 'app-create-swrelease',
  templateUrl: './create-swrelease.component.html',
  styleUrls: ['./create-swrelease.component.scss']
})
export class CreateSwreleaseComponent {

  name:string = "";
  revision:string = "";
  type:string = "";
  filename:string = "";
  isEnabled:boolean = true;
  content: string| ArrayBuffer|any;
  length: number = 0;

  createSwReleaseForm: FormGroup;
  constructor(private fb: FormBuilder, private http: HttpService) {
    this.createSwReleaseForm = fb.group({
      fwreleasename:''
    });
  }

  async extractZip(file: File) {
    const zip = new JSZip();
    const extractedFiles = await zip.loadAsync(file);
    console.log(extractedFiles);
    extractedFiles.forEach(async (relativePath, file) => {
      //console.log(file);
      if(file.name == "unity.app") {
        const content = await file.async("string");
        //console.log(content);
        this.parseXML(content);
      }
      //saveAs(content, relativePath);
    });
  }
  
  parseXML(data:any) {  
    return new Promise(resolve => {  
      var k: string | number,  
        arr:any = [],  
        parser = new xml2js.Parser(  
          {  
            trim: true,  
            explicitArray: true
          });

          parser.parseString(data, (err:any, result:any) => {  
            let obj = JSON.stringify(result);
            let elm = JSON.parse(obj);
            //console.log(elm["app:application"]["$"]["name"]);
            this.name = elm["app:application"]["$"]["name"];
            this.revision= elm["app:application"]["$"]["revision"];
            this.type = elm["app:application"]["$"]["type"]
            this.isEnabled = false;
            //console.log(this.name + " " + this.type + " " + this.revision);
            
            resolve(arr);  
          });  
        });  
  }

  onSubmit() {
    let filepath = this.createSwReleaseForm.value.fwreleasename;
    this.filename = filepath.substring(filepath.lastIndexOf('\\') + 1 );

    if(this.filename == filepath) {
      this.filename = filepath.substring(filepath.lastIndexOf('/') + 1 );
    }

    let request = {
      "filename": this.filename as string,
      "name": this.name as string,
      "revision": this.revision as string,
      "createdon": new Date() as Date,
      "product": this.type as string,
      "length": this.length as number,
      "content": btoa(this.content).toString(),
      //"content": this.content,
      //"length": this.length as number
    }

    //console.log(request.content);
    this.http.uploadsoftwarerelease(JSON.stringify(request)).subscribe((response: string) => {
      let result = JSON.parse(JSON.stringify(response));
      //console.log(result);
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

    //this.filename = event.target.files[0];
    this.extractZip(event.target.files[0]);

    
    const fileReader = new FileReader();
    fileReader.onload = (event) => {
      let binaryData = event.target?.result;
      //console.log(binaryData);
    }

    fileReader.onloadend = (event) => {
      //console.log(fileReader.result);
      this.content = event.target?.result;
      this.length = event.total;
      //console.log(event);
    }

    fileReader.onerror = (event) => {
      console.log(event);
    }
  
    fileReader.readAsBinaryString(event.target.files[0]);
    
  }
}
