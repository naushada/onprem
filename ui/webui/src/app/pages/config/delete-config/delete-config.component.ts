import { Component, OnInit } from '@angular/core';
import { HttpService } from 'src/common/http.service';

@Component({
  selector: 'app-delete-config',
  templateUrl: './delete-config.component.html',
  styleUrls: ['./delete-config.component.scss']
})
export class DeleteConfigComponent implements OnInit {

  selected:Array<any> = [];
  configs:Array<any> = Array<any>();
  constructor(private http: HttpService) {

  }

  ngOnInit(): void {
    this.onClick();  
  }

  onClick() {
    this.http.gettemplate().subscribe((rsp: string) => {
      this.configs.length = 0;
      let result = JSON.parse(JSON.stringify(rsp));
      if(result["status"] == "success") {
        let payload = JSON.parse(result["response"]);
        this.configs = payload;
      }
    },
    (error) => {

    },
    () => {

    });
  }

  onSelectionChanged(event:any) {

  }

  onDelete() {
    let req:any={};
    this.selected.forEach(ent => {
      console.log(ent);
      req.filename = ent.filename;
      req.productmodel = ent.productmodel;
      req.fwversion = ent.fwversion;
      req.createdon = ent.createdon;

    });

    if(this.selected.length) {
      this.http.deletetemplate(JSON.stringify(this.selected)).subscribe(rsp => {
        let result = JSON.parse(JSON.stringify(rsp));
      
        if(result["status"] == "success") {
          alert("Deleted Released Template Successfully");
          //this.onClick();
        }
      },
      (error) => {},
      () => {});
    }
  }
}
